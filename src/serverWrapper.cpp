#include "uv.h"
#include "new"
#include "quickjs/quickjs.h"
#include "quickjs/cutils.h"
#include "quickjs/quickjs-libc.h"
#include <string.h>
#include "Log.hpp"
static JSClassID  js_uv_FILE_class_id;

static JSValue js_uv_FILE_close     (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_uv_FILE_init      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_uv_FILE_read      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_uv_FILE_write     (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static void    js_uv_FILE_finalizer (JSRuntime *rt, JSValueConst this_val);

static const JSCFunctionListEntry js_uv_FILE_proto[] = {
    // for maximum performance, there must be zero overhead and direct low-level calls and no abstraction
	JS_CFUNC_DEF("close", 0, js_uv_FILE_close), // cb:function
	JS_CFUNC_DEF("read",  3, js_uv_FILE_read),// offset:number,size:number,cb:function
	JS_CFUNC_DEF("write", 1, js_uv_FILE_write),// data:string{,offset:number,cb:function}
};

static JSClassDef js_uv_FILE_class = {
    "File",
    .finalizer = js_uv_FILE_finalizer,
    .gc_mark=NULL,
    .call=NULL,
    .exotic=NULL
};

static JSClassID  js_uv_TCP_class_id;
static JSValue js_uv_TCP_init       (JSContext *ctx, JSValueConst func_obj, JSValueConst this_val, int argc, JSValueConst *argv, int flags);
static JSValue js_uv_TCP_close      (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static void    js_uv_TCP_finalizer  (JSRuntime *rt,  JSValueConst this_val);
//static const JSCFunctionListEntry js_uv_TCP_proto[] = {
//	JS_CFUNC_DEF("close", 0, js_uv_TCP_close),
//};
//static JSClassDef js_uv_TCP_class = {
//    "TCP",
//    .finalizer = js_uv_TCP_finalizer,
//    .gc_mark=NULL,
//    .call=js_uv_TCP_init,
//    .exotic=NULL
//};


static const JSCFunctionListEntry js_uv_funcs[] = {
	// openFile(Addr:string, Flags:string[, cb:function]):any ;
	JS_CFUNC_DEF("openFile", 2, js_uv_FILE_init),
};

static int js_uv_init(JSContext *ctx, JSModuleDef *m)
{
	/* File class */

    JSValue file_proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, file_proto, js_uv_FILE_proto, countof(js_uv_FILE_proto));
	JS_NewClassID(&js_uv_FILE_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_uv_FILE_class_id, &js_uv_FILE_class);
    JS_SetClassProto(ctx, js_uv_FILE_class_id, file_proto);

    //JSValue tcp_proto = JS_NewObject(ctx);
	//JS_SetPropertyFunctionList(ctx, tcp_proto, js_uv_TCP_proto, countof(js_uv_TCP_proto));
	//JS_NewClassID(&js_uv_TCP_class_id);
    //JS_NewClass(JS_GetRuntime(ctx), js_uv_TCP_class_id, &js_uv_TCP_class);
    //JS_SetClassProto(ctx, js_uv_TCP_class_id, tcp_proto);


	return JS_SetModuleExportList(ctx,m,js_uv_funcs,countof(js_uv_funcs));
}
// JS_MODULE
JS_MODULE JSModuleDef *js_init_module_uv(JSContext *ctx, const char *module_name)
{
	JSModuleDef *m = JS_NewCModule(ctx, module_name, js_uv_init);
	if (!m)
		return NULL;
	// JS_AddModuleExportList(ctx, m, js_uv_proto_timer, countof(js_uv_proto_timer));
	JS_AddModuleExportList(ctx, m, js_uv_funcs, countof(js_uv_funcs));
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
    //ssize_t refrence_count; replaced by built-in variable refrence count
	JSContext *ctx;
    JSValue this_val;
    uv_file F;

    //uv_buf_t buffer;
};





static JSValue js_uv_FILE_init(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    Console::log(Console::developer,"Log: js_uv_FILE_init\n");

    int res=1;
    FileSystem *stream_ctx;
    Event *event_ctx;
    const char *arg_temp;
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
        if (!JS_IsString(argv[1]))  goto invalide_arg;
        if( (arg_temp = JS_ToCStringLen(ctx, &len, argv[1])) == NULL) return JS_ThrowTypeError(ctx,"invalide arguments");
        for(size_t i=0;i<len;i++){
            switch(arg_temp[i]){
                case 'r':flag_temp|=1;break;
                case 'w':flag_temp|=2;break;
                case '+':flag_temp|=4;break;
                case 'a':mode|=O_TRUNC;break;
                case 't':mode|=O_TMPFILE;break;
                default:
                    Console::log(Console::debug,"Debug: invalide flag: ",arg_temp[i],"\n");
            }
        }
        JS_FreeCString(ctx, arg_temp);
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
    //stream_ctx->this_val = JS_NewObjectClass(ctx, 1?js_uv_FILE_class_id:js_uv_DIR_class_id);
    stream_ctx->this_val = JS_NewObjectClass(ctx, js_uv_FILE_class_id);

    //if(is_file){
        stream_ctx->F=-1;
    //}else{
    //    stream_ctx->system.D=NULL;
    //}
    JS_SetOpaque(stream_ctx->this_val, stream_ctx);




    event_ctx=(Event*)js_malloc(ctx,sizeof(Event));
    if (!event_ctx){
        return JS_ThrowOutOfMemory(ctx);
    }



    if( (arg_temp = JS_ToCStringLen(ctx, &len, argv[0])) == NULL) return JS_ThrowTypeError(ctx,"invalide arguments");

    //if(1)
        res=uv_fs_open(uv_default_loop(), &event_ctx->request, arg_temp, flag, mode, onFileOpen);
    //else
    //    res=uv_fs_opendir(uv_default_loop(), &event_ctx->request,arg_temp,onDirOpen);

    JS_FreeCString(ctx, arg_temp);



    if(res){
        js_free(ctx, event_ctx);
        return JS_ThrowInternalError(ctx, "uv_fs_open failed: %d",res);
    }else{
        event_ctx->request.data=stream_ctx;
        // if is DIR or has 3 argument then the last arg must be a callback
        //event_ctx->onEvent = (!is_file || (argc>2))?JS_DupValue(ctx, argv[argc-1]):JS_UNDEFINED;
        event_ctx->onEvent = JS_DupValue(ctx, argv[argc-1]);
        JS_DupValue(ctx, stream_ctx->this_val);
        return stream_ctx->this_val;
    }
invalide_arg:
    return JS_ThrowTypeError(ctx, "invalide arguments");
}


static void onFileOpen(uv_fs_s *req)
{
    JSValue err[1]={JS_UNDEFINED};
    Event *event_ctx=(Event *)req;
    FileSystem *stream_ctx=(FileSystem *)req->data;

    Console::log(Console::developer,"Dev: onFileOpen\n");
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





static JSValue js_uv_FILE_close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    FileSystem *stream_ctx=NULL;

    Console::log(Console::developer,"js_uv_FILE_close");
    //if it was any thing but 1 or 0
    if (argc&(~1)) goto invalid_arg;
    if (argc && !JS_IsFunction(ctx,argv[0])) goto invalid_arg;

    //stream_ctx = (FileSystem *)JS_GetOpaque(this_val,is_file?js_uv_FILE_class_id:js_uv_DIR_class_id);
    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_FILE_class_id);

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
    }else{
        event_ctx->onEvent=argc>0?JS_DupValue(ctx, argv[0]):JS_UNDEFINED;
        JS_DupValue(ctx, this_val);
        return JS_UNDEFINED;
    }
bad_init:
    return JS_ThrowInternalError(ctx, "not initialized");
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


    Console::log(Console::developer,"Dev: onClose\n");
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

    //NOTE: js_uv_FILE_finalizer is called imedietly whene JS_FreeValue is called
    // so you must have set open_req.result (A.K.A. FD) to invalide FD
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}



static void js_uv_FILE_finalizer (JSRuntime *rt, JSValueConst this_val)
{
    int res;
    Event *event_ctx;
    FileSystem *stream_ctx=NULL;
    Console::log(Console::developer,"Dev: js_uv_FILE_finalizer\n");

    //if(JS_GetClassID(this_val) == js_uv_DIR_class_id){
    //    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_DIR_class_id);
    //    if(stream_ctx->system.D==NULL) goto end;
    //}else{
        stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_FILE_class_id);
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

static JSValue js_uv_FILE_read (JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    FileSystem *stream_ctx=NULL;
    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_FILE_class_id);
    Console::log(Console::developer,"Dev: js_uv_FILE_read\n");


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
        return JS_ThrowInternalError(ctx, "FILE is not initilized currectly");
    }

    return JS_UNDEFINED;
}

static void onRead(uv_fs_t* req)
{
    IOEvent *event_ctx=NULL;
    FileSystem *stream_ctx=NULL;
    JSValue args[2];
    JSValue ret;

    event_ctx=(IOEvent *)req;
    stream_ctx=(FileSystem *)req->data;


    Console::log(Console::developer,"Dev: onRead\n");
    if(stream_ctx==NULL || stream_ctx->ctx==NULL)
    {
        Console::log(Console::error,"Error: FILE was initialized unsuccessfuly\n");
        exit(1);
    }

    args[0]= req->result<0 ? JS_NewString(stream_ctx->ctx,uv_strerror(req->result)) : JS_UNDEFINED;
    args[1]= req->result<0 ? JS_UNDEFINED : JS_NewStringLen(stream_ctx->ctx,event_ctx->bufferPtr->base,req->result);

    ret = JS_Call(stream_ctx->ctx, event_ctx->onEvent, stream_ctx->this_val, 2, args);
    if (JS_IsException(ret)) js_std_dump_error(stream_ctx->ctx);

    JS_FreeValue(stream_ctx->ctx, ret);
    JS_FreeValue(stream_ctx->ctx, args[0]);
    JS_FreeValue(stream_ctx->ctx, args[1]);

    uv_fs_req_cleanup(req);
    JS_FreeValue(stream_ctx->ctx, event_ctx->onEvent);
    js_free(stream_ctx->ctx,event_ctx);

    // WARN: JS_FreeValue may call js_uv_FILE_finalizer (if no refrence exists)
    // so there is no guaranty that 'stream_ctx' will be available
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}

static JSValue js_uv_FILE_write(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    FileSystem *stream_ctx=NULL;
    int64_t offset=-1;

    stream_ctx = (FileSystem *)JS_GetOpaque(this_val,js_uv_FILE_class_id);
    Console::log(Console::developer,"Dev: js_uv_FILE_write\n");

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
        int res=1;
        IOEvent *event_ctx=(IOEvent*)js_malloc(ctx,sizeof(IOEvent));
        event_ctx->request.data=stream_ctx;
        event_ctx->bufferPtr[0].base=(char *)JS_ToCStringLen(ctx,&(event_ctx->bufferPtr[0].len),argv[1]);
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
        return JS_ThrowInternalError(ctx, "FILE is not initilized currectly");
    }


    return JS_UNDEFINED;
}

static void onWrite(uv_fs_t* req)
{
    IOEvent *event_ctx=NULL;
    FileSystem *stream_ctx=NULL;

    event_ctx=(IOEvent *)req;
    stream_ctx=(FileSystem *)req->data;


    Console::log(Console::developer,"Dev: onWrite\n");
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

    // WARN: JS_FreeValue may call js_uv_FILE_finalizer (if no refrence exists)
    // so there is no guaranty that 'stream_ctx' will be available
    JS_FreeValue(stream_ctx->ctx, stream_ctx->this_val);
}

















struct TCP
{
    //ssize_t refrence_count; replaced by built-in variable refrence count
	JSContext *ctx;
    JSValue this_val;
    uv_tcp_s s;

    //uv_buf_t buffer;
};
