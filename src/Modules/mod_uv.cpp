#include "./mod_uv.hpp"
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
static JSValue js_uv_file_close     (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_uv_file_read      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_uv_file_write     (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static void    js_uv_file_finalizer (JSRuntime *rt, JSValueConst this_val);

static const JSCFunctionListEntry js_uv_file_proto[] = {
    // for maximum performance, there must be zero overhead and direct low-level calls and no abstraction
	JS_CFUNC_DEF("close", 0, js_uv_file_close), // cb:function
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




static JSClassID js_uv_sockaddr_class_id;
static JSValue   js_uv_sockaddr_init       (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
static void      js_uv_sockaddr_finalizer  (JSRuntime *rt,  JSValueConst this_val);
static const char js_uv_sockaddr_class_name[]="sockaddr";
static JSClassDef js_uv_sockaddr_class = {
    js_uv_sockaddr_class_name,
    .finalizer = js_uv_sockaddr_finalizer,
    .gc_mark=NULL,
    // note: this is not constructor!
    .call=NULL,
    .exotic=NULL
};



static JSClassID js_uv_udp_class_id;
static JSValue   js_uv_udp_init            (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
static void      js_uv_udp_finalizer       (JSRuntime *rt,  JSValueConst this_val);
static JSValue   js_uv_udp_bind            (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_set_broadcast   (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_start_recv      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue   js_uv_udp_srop_recv       (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static const JSCFunctionListEntry js_uv_udp_proto[] = {
	JS_CFUNC_DEF("startRecv", 2, js_uv_udp_start_recv),
	JS_CFUNC_DEF("stopRecv", 2, js_uv_udp_srop_recv),
	JS_CFUNC_DEF("bind", 2, js_uv_udp_bind),
	JS_CFUNC_DEF("setBroadcast", 1, js_uv_udp_set_broadcast),
};
static const JSCFunctionListEntry js_uv_udp_funcs[] = {
};
static const char js_uv_udp_class_name[]="udp";
static JSClassDef js_uv_udp_class = {
    js_uv_udp_class_name,
    .finalizer = js_uv_udp_finalizer,
    .gc_mark=NULL,
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



    JS_NewClassID(&js_uv_sockaddr_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_uv_sockaddr_class_id, &js_uv_sockaddr_class);
    JSValue sockaddr_proto = JS_NewObject(ctx);
    JSValueConst sockaddr_func = JS_NewCFunction2(ctx, js_uv_sockaddr_init, js_uv_sockaddr_class_name, 2, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, sockaddr_func, sockaddr_proto);
    JS_SetClassProto(ctx, js_uv_sockaddr_class_id, sockaddr_proto);



	JS_NewClassID(&js_uv_udp_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_uv_udp_class_id, &js_uv_udp_class);
    // create, initialize and set a object as the class prototype
    JSValue udp_proto = JS_NewObject(ctx);
    // properties
	JS_SetPropertyFunctionList(ctx, udp_proto, js_uv_udp_proto, countof(js_uv_udp_proto));
    // constructorjs_uv_udp_file_name
    JSValueConst udp_func = JS_NewCFunction2(ctx, js_uv_udp_init, js_uv_udp_class_name, 1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, udp_func, udp_proto);
    // static properties
    //JS_SetPropertyFunctionList(ctx, udp_func, js_uv_udp_funcs, countof(js_uv_udp_funcs));
    JS_SetClassProto(ctx, js_uv_udp_class_id, udp_proto);


    // constructor must be added to module functions
    JS_SetModuleExport(ctx, m, js_uv_file_class_name, file_func);
    JS_SetModuleExport(ctx, m, js_uv_sockaddr_class_name, sockaddr_func);
    JS_SetModuleExport(ctx, m,  js_uv_udp_class_name,  udp_func);
	return JS_SetModuleExportList(ctx,m,js_uv_funcs,countof(js_uv_funcs));
}
//JS_MODULE
//JS_MODULE
JSModuleDef *js_init_module_uv(JSContext *ctx, const char *module_name)
{
	JSModuleDef *m = JS_NewCModule(ctx, module_name, js_uv_init);
	if (!m)
		return NULL;
	// JS_AddModuleExportList(ctx, m, js_uv_proto_timer, countof(js_uv_proto_timer));
	JS_AddModuleExportList(ctx, m, js_uv_funcs, countof(js_uv_funcs));
	JS_AddModuleExport(ctx, m, js_uv_file_class_name);
	JS_AddModuleExport(ctx, m, js_uv_sockaddr_class_name);
	JS_AddModuleExport(ctx, m, js_uv_udp_class_name);
	return m;
}

















static void onFileOpen              (uv_fs_t *req);
static void onClose                 (uv_fs_t* req);
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


//    if(argc!=2){
//        //if is DIR then must has 2 argumunt
//        if(!is_file) goto invalide_arg;
//        //only 3 is another possible situation
//        if(argc!=3)  goto invalide_arg;
//        //if is 3, last one must be function
//        if (!JS_IsFunction(ctx,argv[2])) goto invalide_arg;
//    }else{
//        //if is 2 and is DIR last one must be callback
//        if(!is_file && !JS_IsFunction(ctx,argv[1])) goto invalide_arg;
//    }

    if(argc!=3)  goto invalide_arg;
    if(!JS_IsString(argv[0]))  goto invalide_arg;
    if(!JS_IsFunction(ctx,argv[2])) goto invalide_arg;

    //if(is_file)
    {
        int32_t flag_temp=0;
        if (!JS_IsString(argv[1]))
            goto invalide_arg;
        if( (arg_temp = JS_ToCStringLenRaw(ctx, &len, argv[1])) == NULL)
            goto invalide_arg;
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
                    Console::log(Console::debug,"Debug: invalide flag: ",arg_temp[i],"\n");
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
    if (!event_ctx){
        JS_FreeValue(ctx, thiz);
        return JS_ThrowOutOfMemory(ctx);
    }
    event_ctx->onEvent=JS_UNDEFINED;



    if((arg_temp = JS_ToCStringLenRaw(ctx, &len, argv[0])) == NULL)
        return JS_ThrowTypeError(ctx,"invalide arguments");

    //if(1)
        res=uv_fs_open(uv_default_loop(), &event_ctx->request, (const char *)arg_temp, flag, mode, onFileOpen);
    //else
    //    res=uv_fs_opendir(uv_default_loop(), &event_ctx->request,arg_temp,onDirOpen);



    if(res)
    {
        js_free(ctx, event_ctx);
        JS_FreeValue(ctx, thiz);
        return JS_ThrowInternalError(ctx, "uv_fs_open failed: %d",res);
    }



    event_ctx->request.data=stream_ctx;
    // if is DIR or has 3 argument then the last arg must be a callback
    //event_ctx->onEvent = (!is_file || (argc>2))?JS_DupValue(ctx, argv[argc-1]):JS_UNDEFINED;
    event_ctx->onEvent = JS_DupValue(ctx, argv[argc-1]);
    JS_DupValue(ctx, stream_ctx->this_val);
    return thiz;

invalide_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
}


static void onFileOpen(uv_fs_s *req)
{
    JSValue err[1]={JS_UNDEFINED};
    Event *event_ctx=(Event *)req;
    FileSystem *stream_ctx=(FileSystem *)req->data;

    if(stream_ctx==NULL || stream_ctx->ctx==NULL){
        Console::log(Console::crit,"CRIT: FileSystem was initialized unsuccessfuly\n");
        exit(1);
    }

    if(req->result < 0)
        err[0]=JS_NewString(stream_ctx->ctx,uv_strerror(req->result));
    // 'result' contains file descriptor or negative error number


    stream_ctx->F=req->result;




    //printf("error %s\n",uv_strerror(req->result));
    //if(JS_IsFunction(stream_ctx->ctx,event_ctx->onEvent))
    {
        JSValue ret;

        ret = JS_Call(stream_ctx->ctx, event_ctx->onEvent, JS_UNDEFINED, 1, err);
        if (unlikely(JS_IsException(ret))) js_std_dump_error(stream_ctx->ctx);

        JS_FreeValue(stream_ctx->ctx, ret);
    }
    //stream_ctx->onOpen=JS_UNDEFINED;


    uv_fs_req_cleanup(req);
    JS_FreeValue(stream_ctx->ctx, err[0]);
    JS_FreeValue(stream_ctx->ctx, event_ctx->onEvent);
    js_free(stream_ctx->ctx,event_ctx);

    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}





static JSValue js_uv_file_close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    FileSystem *stream_ctx=NULL;

    //if it was any thing but 1 or 0
    if (argc&(~1)) goto invalid_arg;
    if (argc && !JS_IsFunction(ctx,argv[0])) goto invalid_arg;

    //stream_ctx = (FileSystem *)JS_GetOpaque(this_val,is_file?js_uv_file_class_id:js_uv_DIR_class_id);
    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_file_class_id);

    //if(is_file){
        if(stream_ctx->F<0) goto bad_init;
    //}else
    //    if(stream_ctx->system.D==0)goto bad_init;

    int res;
    Event *event_ctx;

    event_ctx=(Event*)js_malloc(ctx,sizeof(Event));
    event_ctx->request.data=stream_ctx;

    //if(is_file){
        res=uv_fs_close   (uv_default_loop(), &event_ctx->request, stream_ctx->F, onClose);
        stream_ctx->F=-1;
    //}else{
    //    res=uv_fs_closedir(uv_default_loop(), &event_ctx->request, stream_ctx->system.D, onClose);
    //    stream_ctx->system.D=NULL;
    //}




    if(res){
        js_free(ctx,event_ctx);
        return JS_ThrowInternalError(ctx, "uv_fs_close failed: %d",res);
    }

    event_ctx->onEvent=argc>0?JS_DupValue(ctx, argv[0]):JS_UNDEFINED;
    JS_DupValue(ctx, this_val);
    return JS_UNDEFINED;
bad_init:
    return JS_ThrowInternalError(ctx, "not initialized currectly");
invalid_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
}

// uv_strerror(uv_last_error(loop)
void onClose(uv_fs_t* req)
{
    Event *event_ctx=NULL;
    FileSystem *stream_ctx=NULL;

    event_ctx=(Event *)req;
    stream_ctx=(FileSystem *)req->data;


    if(stream_ctx==NULL || stream_ctx->ctx==NULL)
    {
        Console::log(Console::error,"Error: FileSystem was initialized unsuccessfuly\n");
        exit(1);
    }


    //if(JS_IsFunction(stream_ctx->ctx,event_ctx->onEvent))
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
    JS_FreeValue(stream_ctx->ctx, event_ctx->onEvent);
    js_free(stream_ctx->ctx,event_ctx);

    //NOTE: js_uv_file_finalizer is called imedietly whene JS_FreeValue is called
    // so you must have set open_req.result (A.K.A. FD) to invalide FD
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}



static void js_uv_file_finalizer (JSRuntime *rt, JSValueConst this_val)
{
    int res;
    Event *event_ctx;
    FileSystem *stream_ctx=NULL;

    //if(JS_GetClassID(this_val) == js_uv_DIR_class_id){
    //    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_DIR_class_id);
    //    if(stream_ctx->system.D==NULL) goto end;
    //}else{
        stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_file_class_id);
        if(stream_ctx->F<0)     goto end;
    //}

    event_ctx=(Event*)js_malloc_rt(rt,sizeof(Event));
    event_ctx->request.data=rt;
    event_ctx->onEvent=JS_UNDEFINED;

    //if(JS_GetClassID(stream_ctx->this_val) == js_uv_DIR_class_id)
    //    res=uv_fs_closedir(uv_default_loop(), &event_ctx->request, stream_ctx->system.D, onForceClose);
    //else
        res=uv_fs_close(uv_default_loop(), &event_ctx->request, stream_ctx->F, onForceClose);
    if(unlikely(res)) {
        js_free_rt(rt,event_ctx);
        Console::log(Console::warn,"Warn: finalizer fs close failed\n");
    }
end:
    js_free_rt(rt, stream_ctx);
}

static void onForceClose(uv_fs_t* req){
    //puts("LOG: onForceClose");
    //Event *event_ctx=(Event *)req;
    if(req->result<0){
        Console::log(Console::notice,"Note: finalizer fs close cb failed\n");
    }
    uv_fs_req_cleanup(req);
    // must not causes problem
    js_free_rt((JSRuntime *)(req->data),req);
}

static JSValue js_uv_file_read (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    FileSystem *stream_ctx=NULL;
    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_file_class_id);


    int64_t offset=0,size=0;


    if(argc!=3)
		return JS_ThrowTypeError(ctx, "invalide arguments");
    if(!JS_IsFunction(ctx,argv[2]))
		return JS_ThrowTypeError(ctx, "invalide arguments");

    if(JS_ToInt64(ctx,&offset,argv[0]) || JS_ToInt64(ctx,&size,argv[1])){
        return JS_ThrowTypeError(ctx, "JS_ToInt64 failed");
    }

    if(size<1){
        return JS_ThrowRangeError(ctx, "invalid size %ld",size);
    }

    //printf("LOG: read(%ld,%ld)\n",offset,size);
    if(stream_ctx->F>=0)
    {
        int res;
        IOEvent *event_ctx=(IOEvent*)js_malloc(ctx,sizeof(IOEvent)+size);
        event_ctx->request.data=stream_ctx;
        event_ctx->bufferPtr[0]=uv_buf_init(event_ctx->bufferData, size);
        res=uv_fs_read(uv_default_loop(), &event_ctx->request, stream_ctx->F, event_ctx->bufferPtr, 1, offset, onRead);
        if(res){
            js_free(ctx,event_ctx);
            return JS_ThrowInternalError(ctx, "uv_fs_read failed");
        }else{
            event_ctx->onEvent = JS_DupValue(ctx, argv[2]);
            JS_DupValue(ctx, this_val);
        }
    }else{
        return JS_ThrowInternalError(ctx, "file is not initilized currectly");
    }

    return JS_UNDEFINED;
}



void free_file_buffer(JSRuntime *rt, void *opaque, void *ptr)
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


    if(stream_ctx==NULL || stream_ctx->ctx==NULL)
    {
        Console::log(Console::error,"Error: file was initialized unsuccessfuly\n");
        exit(1);
    }

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
		return JS_ThrowTypeError(ctx, "invalide arguments");
    if(!JS_IsString(argv[0]))
		return JS_ThrowTypeError(ctx, "invalide arguments");

    if(argc>1){
        if(!JS_IsFunction(ctx,argv[argc-1]))
            return JS_ThrowTypeError(ctx, "invalide arguments");
        if(argc>2)
            if(JS_ToInt64(ctx,&offset,argv[1]))
                return JS_ThrowTypeError(ctx, "invalide arguments");
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
            return JS_ThrowTypeError(ctx, "invalide arguments");
        }
        res=uv_fs_write(uv_default_loop(), &event_ctx->request, stream_ctx->F, event_ctx->bufferPtr, 1, offset, onWrite);
        if(res){
            js_free(ctx,event_ctx);
            return JS_ThrowInternalError(ctx, "uv_fs_read failed");
        }else{
            event_ctx->onEvent = argc>1 ? JS_DupValue(ctx, argv[argc-1]) : JS_UNDEFINED;
            JS_DupValue(ctx, this_val);
        }
    }else{
        return JS_ThrowInternalError(ctx, "file is not initilized currectly");
    }


    return JS_UNDEFINED;
}

static void onWrite(uv_fs_t* req)
{
    IOEvent *event_ctx=NULL;
    FileSystem *stream_ctx=NULL;

    event_ctx=(IOEvent *)req;
    stream_ctx=(FileSystem *)req->data;


    if(stream_ctx==NULL || stream_ctx->ctx==NULL)
    {
        Console::log(Console::error,"Error: File was initialized unsuccessfuly\n");
        exit(1);
    }

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


















struct sockAddr
{
    struct sockaddr_in addr;
};

static JSValue js_uv_sockaddr_init (JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv)
{
    sockAddr *thiz_ctx=nullptr;
    int32_t port=0,res=0;
    size_t len=0;
    const char *arg_temp=nullptr;
    JSValue thiz;


    if(argc!=2)
    {
        return JS_ThrowTypeError(ctx, "invalide arguments");
    }
    if(JS_ToInt32(ctx,&port,argv[1]))
    {
        return JS_ThrowTypeError(ctx, "invalide 2nd arguments");
    }

    thiz_ctx=(sockAddr*)js_malloc(ctx, sizeof(sockAddr));
    if (!thiz_ctx)
        return JS_ThrowOutOfMemory(ctx);

    if((arg_temp = JS_ToCStringLen(ctx, &len, argv[0])) == NULL)
        return JS_ThrowTypeError(ctx, "invalide 1st arguments");
    res=uv_ip4_addr(arg_temp, port, &thiz_ctx->addr);
    JS_FreeCString(ctx, arg_temp);
    if(res)
    {
        js_free(ctx,thiz_ctx);
        return JS_ThrowInternalError(ctx, "uv_ip4_addr: %s", uv_strerror(res));
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

struct UDP
{
    uv_udp_s s;
	JSContext *ctx;
    JSValue this_val;
    JSValue recvcb;
    JSValue sendcb;
    JSValue socket_address;
};

static JSValue js_uv_udp_init(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
    UDP *stream_ctx;
    int32_t res;
    JSValue thiz;

    if(argc!=0)  goto invalide_arg;

    stream_ctx=(UDP*)js_malloc(ctx, sizeof(UDP));
    if (!stream_ctx)
        return JS_ThrowOutOfMemory(ctx);

    res = uv_udp_init(uv_default_loop(),&stream_ctx->s);
    if(res)
    {
        js_free(ctx,stream_ctx);
        return JS_ThrowInternalError(ctx,"uv_udp_init: %s", uv_strerror(res));
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
    stream_ctx->sendcb=JS_UNDEFINED;
    stream_ctx->socket_address=JS_UNDEFINED;
    JS_SetOpaque(thiz, stream_ctx);
    return thiz;
invalide_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
}


static void js_uv_udp_finalizer(JSRuntime *rt, JSValueConst this_val)
{
    UDP *stream_ctx;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    uv_close((uv_handle_t*) &stream_ctx->s, NULL);
    JS_FreeValueRT(rt,stream_ctx->socket_address);
    js_free_rt(rt,stream_ctx);
}




static JSValue js_uv_udp_bind(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
    UDP *stream_ctx=nullptr;
    sockAddr *saddr;
    int32_t res=0;

    if(argc!=1
        || (saddr = (sockAddr *)JS_GetOpaque(argv[0],js_uv_sockaddr_class_id)) == NULL)
        goto invalide_arg;

    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    res = uv_udp_bind(&stream_ctx->s, (const struct sockaddr*)&saddr->addr,0);
    if(res)
        return JS_ThrowInternalError(ctx, "uv_udp_bind: %s", uv_strerror(res));


    JS_FreeValue(ctx,stream_ctx->socket_address);
    stream_ctx->socket_address=JS_DupValue(ctx,argv[0]);

    return JS_UNDEFINED;
invalide_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
}


static JSValue js_uv_udp_set_broadcast(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    UDP *stream_ctx=nullptr;
    int val=0;

    if(argc!=1)
        goto invalide_arg;
    val=JS_ToBool(ctx,argv[0]);

    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);
    val=uv_udp_set_broadcast(&stream_ctx->s, val);
    if(val)
        return JS_ThrowInternalError(ctx, "uv_udp_set_broadcast: %s", uv_strerror(val));
    return JS_UNDEFINED;
invalide_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
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
    JSValue args[2];
    if (nread < 1) {
        if(nread != 0){
            fprintf(stderr, "Read error!\n");
        }
        //uv_close((uv_handle_t*) req, NULL);
        goto end;
    }

    //fprintf(stdout, "Recv from %s %d bytes\n", sender, nread, buf->base);
    //fwrite(buf->base, nread, 1, stdout);

    args[0]=JS_NewStringWLen(stream_ctx->ctx, 17);
    str=JS_ToCStringLenRaw(stream_ctx->ctx,nullptr,args[0]);
    if(!str){
        fprintf(stderr, "JS_NewStringWLen failed\n");
        goto end;
    }
    uv_ip4_name((struct sockaddr_in*) addr, (char *)str, 16);
    args[1]=JS_NewArrayBuffer(stream_ctx->ctx, (uint8_t *)buf->base, nread, &free_udp_buffer, nullptr, false);
    // may uv_udp_recv_stop be called by JS which causes to JS_FreeValue(this_val)
    // be called which may causes to js_free_rt(stream_ctx) be called.
    JS_DupValue(stream_ctx->ctx,stream_ctx->this_val);
    ret = JS_Call(stream_ctx->ctx, stream_ctx->recvcb, stream_ctx->this_val, 2, args);
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
    if(argc!=1 || !JS_IsFunction(ctx,argv[0]))  goto invalide_arg;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);


    if(uv_udp_recv_start(&stream_ctx->s,alloc_udp_buffer,on_udp_read)){
        return JS_ThrowInternalError(ctx, "uv_udp_recv_start failed");
    }

    stream_ctx->recvcb=JS_DupValue(ctx,argv[0]);
    JS_DupValue(ctx,this_val);

    return JS_UNDEFINED;

invalide_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
}
static JSValue js_uv_udp_srop_recv(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    UDP *stream_ctx;
    if(argc!=0)  goto invalide_arg;
    stream_ctx = (UDP *)JS_GetOpaque(this_val,js_uv_udp_class_id);

    JS_FreeValue(ctx,stream_ctx->recvcb);
    stream_ctx->recvcb=JS_UNDEFINED;

    if(uv_udp_recv_stop(&stream_ctx->s)){
        return JS_ThrowInternalError(ctx, "uv_udp_recv_start failed");
    }

    JS_FreeValue(ctx,this_val);
    return JS_UNDEFINED;

invalide_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
}















#if IS_LINUX_OS
inline unsigned int
XStringToToKeycode (Display * display, const char * const string) {
  return XKeysymToKeycode(display, XStringToKeysym(string) );
}

int
KeyEvent (const char* const keyname) {
  Display * display = NULL;
  Window window = 0;
  int revert_to_ret = 0;
  XKeyEvent event;
  Status status;

  display = XOpenDisplay(NULL); /* localhost:0.0 */
  if (display == NULL) {
    fprintf(stderr, "Could not open display: localhost:0.0\n");
    return 1;
  }

  /**
  * Target window is the window currently focused.
  */
  XGetInputFocus(display, &window, &revert_to_ret);
  if (window == 0) {
    fprintf(stderr, "Could not detect the window which has gotten input focus.\n");
    return 1;
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
    fprintf(stderr, "Failed to send evnet: type=KeyPress\n");
    return 1; /* error */
  }

  event.type = KeyRelease;
  status = XSendEvent(display, window, 1, KeyReleaseMask, (XEvent *)&event);

  XSync(display, 1);
  XCloseDisplay(display);

  return status;
}
#endif



static JSValue js_uv_simulate_key(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#if IS_WINDOWS_OS
    int c;
    if(argc!=1 || !JS_IsNumber(argv[0]))
        return JS_ThrowTypeError(ctx, "invalide arguments");

    if(JS_ToInt32(ctx,&c,argv[0]))
        return JS_ThrowTypeError(ctx, "invalide arguments");

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

    KeyEvent("a");
#endif
    return JS_UNDEFINED;
}
