package main

/*
#cgo LDFLAGS: -lm -ldl
#cgo CFLAGS: -DCONFIG_VERSION=

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "quickjs.h"

extern JSValue foo_finalizer(JSRuntime *rt, JSValue this);

extern JSValue js_showFoo(JSContext *ctx, JSValueConst this, int argc, JSValueConst *argv);

// helper functions
static JSCFunctionListEntry js_cfunc_def(const char *name, uint8_t length, JSCFunction *func1)
{
	struct JSCFunctionListEntry f = {
		name,
		JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE,
		JS_DEF_CFUNC,
		0,
		.u = {
			.func = {
				length,
				JS_CFUNC_generic,
				{
					.generic = func1
				}
			}
		}
	};
	return f;
}

static void *js_value_get_ptr(JSValue v)
{
	return v.u.ptr;
}

static void JS_SetOpaque_uintptr(JSValue obj, uintptr_t opaque)
{
	JS_SetOpaque(obj, (void *)(opaque));
}
*/
import "C"

import (
	"fmt"
	"runtime/cgo"
	"unsafe"
)

var (
	EVAL_GLOBAL int = int(C.JS_EVAL_TYPE_GLOBAL)
	EVAL_MODULE int = int(C.JS_EVAL_TYPE_MODULE)
	EVAL_STRICT int = int(C.JS_EVAL_FLAG_STRICT)
	EVAL_STRIP  int = int(C.JS_EVAL_FLAG_STRIP)
)

type Runtime struct {
	raw *C.JSRuntime
}

func NewRuntime() *Runtime {
	rt := &Runtime{raw: C.JS_NewRuntime()}
	C.JS_SetCanBlock(rt.raw, C.int(1))
	return rt
}

func (rt *Runtime) NewContext() *Context {
	return &Context{raw: C.JS_NewContext(rt.raw)}
}

func (rt *Runtime) RunGC() {
	C.JS_RunGC(rt.raw)
}

func (rt *Runtime) Free() {
	C.JS_FreeRuntime(rt.raw)
}

type Context struct {
	raw *C.JSContext
}

func (ctx *Context) Free() {
	C.JS_FreeContext(ctx.raw)
}

func (ctx *Context) evalFile(code string, evalType int, filename string) Value {
	cCode := C.CString(code)
	defer C.free(unsafe.Pointer(cCode))
	cFilename := C.CString(filename)
	defer C.free(unsafe.Pointer(cFilename))
	return Value{
		ctx: ctx,
		raw: C.JS_Eval(ctx.raw, cCode, C.size_t(len(code)), cFilename, C.int(evalType)),
	}
}

func (ctx *Context) EvalFile(code string, evalType int, filename string) (Value, error) {
	ret := ctx.evalFile(code, evalType, filename)
	return ret, nil
}

func (ctx *Context) Eval(code string, evalType int) (Value, error) {
	return ctx.EvalFile(code, evalType, "<code>")
}

type Value struct {
	ctx *Context
	raw C.JSValue
}

func (v Value) Free() {
	C.JS_FreeValue(v.ctx.raw, v.raw)
}

func (v Value) Context() *Context {
	return v.ctx
}

func (v Value) Bool() bool {
	return C.JS_ToBool(v.ctx.raw, v.raw) == 1
}

func (v Value) String() string {
	cStr := C.JS_ToCString(v.ctx.raw, v.raw)
	defer C.JS_FreeCString(v.ctx.raw, cStr)
	return C.GoString(cStr)
}

func (v Value) GetOpaque() interface{} {
	return GetOpaque_anyclass(v.raw)
}

func GetOpaque_anyclass(v C.JSValueConst) interface{} {
	if v.tag != C.JS_TAG_OBJECT {
		return nil
	}
	// Skip checking the class_id.
	// if (p->class_id != class_id)
	//     return NULL;
	p := C.js_value_get_ptr(v)
	return cgo.Handle(p).Value()
}

type Function func(ctx *C.JSContext, this C.JSValueConst, argc C.int, argv *C.JSValueConst) C.JSValue

func main() {
	fmt.Println("hey")

	rt := NewRuntime()
	defer rt.Free()

	ctx := rt.NewContext()
	defer ctx.Free()

}

type foo struct {
	x int
}

var (
	fooClassID int
)

func showFoo(f *foo) {
	fmt.Println(f.x)
}

func goArgs(argc C.int, argv *C.JSValueConst) []C.JSValueConst {
	return (*[1 << unsafe.Sizeof(0)]C.JSValueConst)(unsafe.Pointer(argv))[:argc:argc]
}

var _ Function = js_showFoo

//export js_showFoo
func js_showFoo(ctx *C.JSContext, this C.JSValueConst, argc C.int, argv *C.JSValueConst) C.JSValue {
	// JS to Go
	// object型ならGetOpaqueして中身のptrを取り出す
	// int, float ならunionされてる他のメンバを使う
	// Stringならptrをstringとして扱う
	// sliceは? -> 単にtype assertionするだけでok
	args := goArgs(argc, argv)
	f := GetOpaque_anyclass(args[0]).(*foo)

	// Call Go
	showFoo(f)

	// Go to JS
	p := &foo{}
	obj := C.JS_NewObjectClass(ctx, C.int(fooClassID))
	C.JS_SetOpaque_uintptr(obj, C.uintptr_t(cgo.NewHandle(p)))
	return obj
}

func foo_finalizer(rt *C.JSRuntime, this C.JSValue) {
	p := C.JS_GetOpaque(this, C.uint(fooClassID))
	cgo.Handle(p).Delete()
}

func finalizer_proxy(rt *C.JSRuntime, this C.JSValue) {
	p := C.JS_GetOpaque(this, C.uint(fooClassID))
	cgo.Handle(p).Delete()
}

// 関数リストの定義がかなりめんどい

// Define foo as JS Class
func defineFoo(ctx *Context) {
	// クラスID払い出し
	var id C.uint
	C.JS_NewClassID(&id)
	fooClassID = int(id)

	// 名前とfinalizer定義
	def := C.JSClassDef{}
	def.class_name = C.CString("foo") // 解放不要?
	def.finalizer = (*C.JSClassFinalizer)(C.foo_finalizer)
	C.JS_NewClass(C.JS_GetRuntime(ctx.raw), C.uint(fooClassID), &def)

	// prototype作ってメソッド定義
	proto := C.JS_NewObject(ctx.raw)
	name := "showFoo"
	cName := C.CString(name)
	defer C.free(unsafe.Pointer(cName))

	methods := []C.JSCFunctionListEntry{
		C.js_cfunc_def(cName, C.uchar(1), (*C.JSCFunction)(unsafe.Pointer(C.js_showFoo))),
	}
	C.JS_SetPropertyFunctionList(ctx.raw, proto, &methods[0], C.int(len(methods)))
	C.JS_SetClassProto(ctx.raw, C.uint(fooClassID), proto)
}
