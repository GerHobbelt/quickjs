#include "uv.h"
#include "quickjs/cutils.h"
#include "quickjs/quickjs-libc.h"
#include "serverWrapper.hpp"
#include <string.h>
#include <stdint.h>
#include "Log.hpp"



static_assert(sizeof(size_t)==8);

struct header {
  // a seperator for better readability in binary files.
  char new_line;
  // 510 character + null terminator
  char fname[511];
  size_t size;
  // load_only means it is a module and int is not required to be executed
  // but import exported functions from module.
  char load_only;
  char new_line2;
} __attribute__((packed)) ;
#define header_size 522
static_assert(sizeof(struct header) == header_size);
#define SNAPSHOT_NAME "out.qjs"
static FILE *fo;
static BOOL byte_swap=0;
static char const * const QJSMAGIC="QJSCARCH001002001";


static int eval_file(JSContext *ctx, const char *filename, int module)
{
    int ret=0;
    JSValue val;
	uint8_t *buf;
	size_t buf_len=0;

    buf = js_load_file(ctx, &buf_len, filename);
    if(buf==NULL){
        ret=-1;
        goto ret;
    }
    if (module < 0)
        module = (has_suffix(filename, ".mjs") || JS_DetectModule((const char *)buf, buf_len));
    if (module) {
        /* for the modules, we compile then run to be able to set import.meta */
        val = JS_Eval(ctx, (const char*)buf, buf_len, filename, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
        if (!JS_IsException(val)) {
            js_module_set_import_meta(ctx, val, TRUE, TRUE);
            val = JS_EvalFunction(ctx, val);
        }
    } else {
        val = JS_Eval(ctx, (const char*)buf, buf_len, filename, JS_EVAL_TYPE_GLOBAL);
    }
    if (JS_IsException(val)) {
        js_std_dump_error(ctx);
        ret = -2;
    }
    JS_FreeValue(ctx, val);
    js_free(ctx, buf);
ret:
	return ret;
}


static int eval_binary(JSContext *ctx, const char *filename)
{
    int ret=-1;
    uint8_t *buffer=NULL,*cur=NULL;
	size_t buf_len=0;
	if(!ctx)
        goto error;
    buffer = js_load_file(ctx, &buf_len, filename);
    cur=(buffer+strlen(QJSMAGIC));
    if (buffer==NULL) {
        Console::log(Console::error,"Error: Unable to read ",filename,"\n");
        return -1;
    }
    if(buf_len>INT32_MAX){
        Console::log(Console::error,"Error: Too large file ",filename," (",buf_len," Byte)\n");goto error;
    }
    if(buf_len < (strlen(QJSMAGIC) + header_size)) {
        Console::log(Console::error,"Error: Too small file ",filename," (",buf_len," Byte)\n");goto error;
    }
    if(memcmp(buffer,QJSMAGIC,strlen(QJSMAGIC))){
        Console::log(Console::error,"Error: Invalide MAGIC header: ",filename,"\n");goto error;
    }
    while((cur-buffer) < (ssize_t)buf_len
          && (buf_len-(cur-buffer)) > header_size)
    {
        struct header *H = (struct header*)cur;
        Console::log(Console::debug,"Debug: executing ",H->fname," (size: ", H->size, H->load_only?",load_only":"", ")\n");
        if((buf_len-(cur-buffer)) < H->size) {
            Console::log(Console::error,"Error: Bad file unexpected ending\n");goto error;
        }
        cur+=header_size;
        js_std_eval_binary(ctx, cur, H->size, H->load_only);
        cur+=H->size;
    }
    ret=0;
error:
    if(buffer!=NULL){
        js_free(ctx, buffer);
    }
	return ret;
}

static void output_object_code(JSContext *ctx, JSValueConst obj, const char *c_name, char load_only)
{
    uint8_t *out_buf;
    size_t out_buf_len;
    int flags;
    flags = JS_WRITE_OBJ_BYTECODE;
    if (byte_swap)
        flags |= JS_WRITE_OBJ_BSWAP;
    out_buf = JS_WriteObject(ctx, &out_buf_len, obj, flags);
    if (!out_buf) {
        js_std_dump_error(ctx);
        exit(1);
    }

    {
        struct header H;
        // in case of security, leaves no foot print of Stack memory on fname parameter.
        memset(&H,0,sizeof(H));
        H.new_line='\n';
        snprintf(H.fname, sizeof(H.fname)-1, "%s", c_name);
        H.size=out_buf_len;
        H.load_only=load_only;
        H.new_line2='\n';

        fwrite(&H, 1, sizeof(H), fo);
        fwrite(out_buf, 1, out_buf_len, fo);
    }

    js_free(ctx, out_buf);
}

static int js_module_dummy_init(JSContext *ctx, JSModuleDef *m)
{
    (void)ctx;(void)m;
    /* should never be called when compiling JS code */
    abort();
}

static JSModuleDef *jsc_module_loader(JSContext *ctx, const char *module_name, void *opaque)
{
    (void)opaque;
    JSModuleDef *m;

    if (has_suffix(module_name, ".so") || has_suffix(module_name, ".dll")) {
        Console::log(Console::debug,"Debug: binary module '",module_name,"' will be dynamically loaded\n");
        /* create a dummy module */
        m = JS_NewCModule(ctx, module_name, js_module_dummy_init);
        /* the resulting executable will export its symbols for the
           dynamic library */
    } else {
        size_t buf_len;
        uint8_t *buf;
        JSValue func_val;

        buf = js_load_file(ctx, &buf_len, module_name);
        if (!buf) {
            JS_ThrowReferenceError(ctx, "could not load module filename '%s'", module_name);
            return NULL;
        }

        /* compile the module */
        func_val = JS_Eval(ctx, (char *)buf, buf_len, module_name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
        js_free(ctx, buf);
        if (JS_IsException(func_val))
            return NULL;
        Console::log(Console::debug,"Debug: jsc_module_loader() -> output_object_code(",module_name,")\n");
        output_object_code(ctx, func_val, module_name, TRUE);

        /* the module is already referenced, so we must free it */
        m = (JSModuleDef *)JS_VALUE_GET_PTR(func_val);
        JS_FreeValue(ctx, func_val);
    }
    return m;
}

static int compile_file(JSContext *ctx, const char *filename, int module)
{
    uint8_t *buf;
    int eval_flags;
    JSValue obj;
    size_t buf_len;

    buf = js_load_file(ctx, &buf_len, filename);
    if (!buf) {
        Console::log(Console::error,"Error: Could not load '",filename,"'\n");
        return 1;
    }
    eval_flags = JS_EVAL_FLAG_COMPILE_ONLY;
    if (module < 0)
        module = (has_suffix(filename, ".mjs") || JS_DetectModule((const char *)buf, buf_len));

    if (module) eval_flags |= JS_EVAL_TYPE_MODULE;
    else        eval_flags |= JS_EVAL_TYPE_GLOBAL;

    obj = JS_Eval(ctx, (const char *)buf, buf_len, filename, eval_flags);
    js_free(ctx, buf);
    if (JS_IsException(obj)) {
        js_std_dump_error(ctx);
        return 1;
    }
    Console::log(Console::debug,"Debug: compile_file() -> output_object_code(",filename,",",!!module,")\n");
    output_object_code(ctx, obj, filename, !!module);
    JS_FreeValue(ctx, obj);
    return 0;
}





































void help(char const * const name)
{
	printf("QuickJS z cusume '" __DATE__"-" __TIME__ "'\n"
			"usage: %s [options [file]] main_file [script arguments]\n"
			"\t-h  --help         list options\n"
			"\t-m  --module       load all files as ES6 module (default=autodetect)\n"
			"\t    --script       load all files as ES6 script (default=autodetect)\n"
			"\t-I  --include file include an additional file(modules)\n"
				"\t\tif you have problem with importing\n"
				"\t\t(eg: overriding a module file with compiled one)\n"
			"\t-S --snapshot execute 'main_file' as compiler binary\n"
			"\t-C --compile create a snapshot of 'main_file' and included files into out.qjs\n"
                "\t\tCompiling only saves your time by avoiding to recompile sources again"
			,name
		);
	return;
}

int main(int argc, char const *argv[])
{
	JSRuntime *rt;
	JSContext *ctx;
	int option_ind;
	int module = -1;
	const char *include_list[32];
	int i, include_count = 0,ret_code=1;
	//running codes:0
    //running snapshot:1
    //compiling to snapshot:2
    static int MainType=0;

	/* cannot use getopt because we want to pass the command line to the script */
	option_ind = 1;

	Console::init();

	while (option_ind < argc && *argv[option_ind] == '-') {
		const char *arg = argv[option_ind] + 1;
		const char *longopt = "";
		/* a single - is not an option, it also stops argument scanning */
		if (!*arg)
			break;
		option_ind++;
		if (*arg == '-') {
			longopt = arg + 1;
			arg += strlen(arg);
			/* -- stops argument scanning */
			if (!*longopt)
				break;
		}
		for (; *arg || *longopt; longopt = "") {
			char opt = *arg;
			if (opt)
				arg++;
			if (opt == 'h' || !strcmp(longopt, "help")) {
				help(argv[0]);
				return 0;
			}
			if (opt == 'I' || !strcmp(longopt, "include")) {
				if (option_ind >= argc) {
					Console::log(Console::crit, "Crit: expecting filename");
					exit(1);
				}
				if (include_count >= (int)countof(include_list)) {
					Console::log(Console::crit, "Crit: too many included files");
					exit(1);
				}
				include_list[include_count++] = argv[option_ind++];
				continue;
			}
			if (opt == 'm' || !strcmp(longopt, "module")) {
				module = 1;
				continue;
			}
			if (!strcmp(longopt, "script")) {
				module = 0;
				continue;
			}
			if (opt == 'C' || !strcmp(longopt, "compile")) {
				MainType = 2;
				continue;
			}
			if (opt == 'S' || !strcmp(longopt, "snapshot")) {
				MainType = 1;
				continue;
			}
			if (opt) {
				Console::log(Console::warn, "Warn: '",argv[0],"' unknown option '-",opt,"'\n");
			} else {
				Console::log(Console::warn, "Warn: '",argv[0],"' unknown option '--",longopt,"'\n");
			}
			help(argv[0]);
		}
	}

	if(option_ind>argc ||  argv[option_ind]==NULL || argv[option_ind][0]==0x0)
	{
		Console::log(Console::crit, "Crit: expecting filename!");fflush(stderr);
		exit(1);
	}

	rt = JS_NewRuntime();
	if (!rt) {Console::log(Console::crit, "Crit: ",argv[0]," cannot allocate JS runtime\n");exit(2);}
	JS_SetMemoryLimit(rt, 10000000);/* limit the memory usage to 'n' bytes (optional) */
	JS_SetMaxStackSize(rt, 500000);/* limit the stack size to 'n' bytes (optional) */

	ctx = JS_NewContext(rt);
	if (!ctx) {Console::log(Console::crit, "Crit: ",argv[0]," cannot allocate JS context\n");exit(2);}
#ifdef CONFIG_BIGNUM
	JS_AddIntrinsicBigFloat(ctx);
	JS_AddIntrinsicBigDecimal(ctx);
	JS_AddIntrinsicOperators(ctx);
	JS_EnableBignumExt(ctx, TRUE);
#endif
	/* system modules */


    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");
    js_init_module_uv(ctx, "uv");
	/* loader for ES6 modules, also used by import() */
    if(MainType==2)
        JS_SetModuleLoaderFunc(rt, NULL, jsc_module_loader, NULL);
    else
        JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);
    js_std_add_helpers(ctx, argc - option_ind, argv + option_ind);

	if(MainType==2){
        fo = fopen(SNAPSHOT_NAME, "wb");
        if (!fo) {
            perror(SNAPSHOT_NAME);goto fail;
        }
        fprintf(fo, QJSMAGIC);
        for(i = 0; i < include_count; i++)
            if (!jsc_module_loader(ctx, include_list[i], NULL)) {
                Console::log(Console::error, "Error: Could not load dynamic module '",include_list[i],"'\n");goto fail;
            }
        if (compile_file(ctx, argv[option_ind], module))
            goto fail;
        fclose(fo);
    }else if(MainType==1){
        if (eval_binary(ctx, argv[option_ind]))
            goto fail;
    }else{
        for(i = 0; i < include_count; i++)
            if (eval_file(ctx, include_list[i], module))
                goto fail;
        if (eval_file(ctx, argv[option_ind], module))
            goto fail;
    }

	//if(MainType!=2){z
		JSContext *ctx1;
		int err;
		while(1) {
			while(0<(err = JS_ExecutePendingJob(JS_GetRuntime(ctx), &ctx1))){}
			if (err < 0) {
				js_std_dump_error(ctx1);
				break;
			}
			if(0==uv_run(uv_default_loop(), UV_RUN_ONCE))
				break;
		}
	//}
	ret_code=0;
 fail:
	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);

	//getchar();
	return ret_code;
}
