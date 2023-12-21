# Tools
CC=clang
EMSDK_VERSION=3.1.50
EMSDK_DOCKER_IMAGE=emscripten/emsdk:3.1.50
EMCC_SRC=../../scripts/emcc.sh
EMCC=EMSDK_VERSION=$(EMSDK_VERSION) EMSDK_DOCKER_IMAGE=$(EMSDK_DOCKER_IMAGE) EMSDK_DOCKER_CACHE=$(REPO_ROOT)/emsdk-cache/$(EMSDK_VERSION) $(EMCC_SRC)
GENERATE_TS_SRC=../../generate.ts
GENERATE_TS=$(GENERATE_TS_ENV) npx ts-node $(GENERATE_TS_SRC)
PRETTIER=npx prettier
THIS_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
REPO_ROOT := $(THIS_DIR)/../..
TSC=../../node_modules/.bin/tsc

DEBUG_MAKE=1

# Paths
QUICKJS_ROOT=../../quickjs
WRAPPER_ROOT=../../c
TS_ROOT=../../ts
BUILD_ROOT=build
BUILD_WRAPPER=$(BUILD_ROOT)/wrapper
BUILD_QUICKJS=$(BUILD_ROOT)/quickjs
BUILD_TS=dist

# QuickJS
QUICKJS_OBJS=quickjs.o libregexp.o libunicode.o cutils.o quickjs-libc.o libbf.o
QUICKJS_CONFIG_VERSION=$(shell cat $(QUICKJS_ROOT)/VERSION)
QUICKJS_DEFINES:=-D_GNU_SOURCE -DCONFIG_VERSION=\"$(QUICKJS_CONFIG_VERSION)\" -DCONFIG_STACK_CHECK -DCONFIG_BIGNUM
VARIANT_QUICKJS_OBJS=$(patsubst %.o, $(BUILD_QUICKJS)/%.o, $(QUICKJS_OBJS))

# quickjs-emscripten
WRAPPER_DEFINES+=-Wcast-function-type   # Likewise, warns about some quickjs casts we don't control.
EMCC_EXPORTED_FUNCS+=-s EXPORTED_FUNCTIONS=@$(BUILD_WRAPPER)/symbols.json
EMCC_EXPORTED_FUNCS_ASYNCIFY+=-s EXPORTED_FUNCTIONS=@$(BUILD_WRAPPER)/symbols.asyncify.json

# Emscripten options
CFLAGS_WASM+=-s WASM=1
CFLAGS_WASM+=-s EXPORTED_RUNTIME_METHODS=@../../exportedRuntimeMethods.json
CFLAGS_WASM+=-s MODULARIZE=1
CFLAGS_WASM+=-s EXPORT_NAME=QuickJSRaw
CFLAGS_WASM+=-s INVOKE_RUN=0
CFLAGS_WASM+=-s ALLOW_MEMORY_GROWTH=1
CFLAGS_WASM+=-s ALLOW_TABLE_GROWTH=1
CFLAGS_WASM+=-s STACK_SIZE=5MB
# CFLAGS_WASM+=-s MINIMAL_RUNTIME=1 # Appears to break MODULARIZE
CFLAGS_WASM+=-s SUPPORT_ERRNO=0

# Emscripten options - like STRICT
# https://github.com/emscripten-core/emscripten/blob/fa339b76424ca9fbe5cf15faea0295d2ac8d58cc/src/settings.js#L1095-L1109
# CFLAGS_WASM+=-s STRICT_JS=1 # Doesn't work with MODULARIZE
CFLAGS_WASM+=-s IGNORE_MISSING_MAIN=0 --no-entry
CFLAGS_WASM+=-s AUTO_JS_LIBRARIES=0
CFLAGS_WASM+=-s -lccall.js
CFLAGS_WASM+=-s AUTO_NATIVE_LIBRARIES=0
CFLAGS_WASM+=-s AUTO_ARCHIVE_INDEXES=0
CFLAGS_WASM+=-s DEFAULT_TO_CXX=0
CFLAGS_WASM+=-s ALLOW_UNIMPLEMENTED_SYSCALLS=0

# Emscripten options - NodeJS
CFLAGS_WASM+=-s MIN_NODE_VERSION=160000
CFLAGS_WASM+=-s NODEJS_CATCH_EXIT=0

# VARIANT
VARIANT=REPLACE_THIS
SYNC=REPLACE_THIS

# Emscripten options - variant specific
CFLAGS_WASM_VARIANT=REPLACE_THIS

# GENERATE_TS options - variant specific
GENERATE_TS_ENV_VARIANT=REPLACE_THIS

ifdef DEBUG_MAKE
	MKDIRP=@echo "\n=====[["" target: $@, deps: $<, variant: $(VARIANT) ""]]=====" ; mkdir -p $(dir $@)
else
	MKDIRP=@mkdir -p $(dir $@)
endif

###############################################################################
# High-level
all: WASM JS

clean-generate:
	rm -rfv $(BUILD_TS)

clean: clean-generate
	rm -rfv $(BUILD_ROOT)

###############################################################################
# Typescript files
JS: $(BUILD_TS)/ffi.js $(BUILD_TS)/ffi.d.ts

$(BUILD_TS)/ffi.js $(BUILD_TS)/ffi.d.ts: tsconfig.json $(BUILD_TS)/ffi.ts
	$(TSC) --project .

$(BUILD_TS)/ffi.ts: $(WRAPPER_ROOT)/interface.c $(GENERATE_TS_SRC) ../quickjs-ffi-types/ffi-types.ts
	$(MKDIRP)
	$(GENERATE_TS) ffi $@

###############################################################################
# Wasm files
WASM: $(BUILD_TS)/emscripten-module.js $(BUILD_TS)/emscripten-module.d.ts GENERATE
GENERATE: $(BUILD_TS)/ffi.ts
WASM_SYMBOLS=$(BUILD_WRAPPER)/symbols.json $(BUILD_WRAPPER)/asyncify-remove.json $(BUILD_WRAPPER)/asyncify-imports.json

$(BUILD_TS)/emscripten-module.js: $(BUILD_WRAPPER)/interface.o $(VARIANT_QUICKJS_OBJS) $(WASM_SYMBOLS) | $(EMCC_SRC)
	$(MKDIRP)
	$(EMCC) $(CFLAGS_WASM) $(WRAPPER_DEFINES) $(EMCC_EXPORTED_FUNCS) -o $@ $< $(VARIANT_QUICKJS_OBJS)

$(BUILD_TS)/emscripten-module.d.ts: $(TS_ROOT)/types-generated/emscripten-module.$(SYNC).d.ts
	echo '// Generated from $<' > $@
	cat $< >> $@

$(BUILD_WRAPPER)/%.o: $(WRAPPER_ROOT)/%.c $(WASM_SYMBOLS) | $(EMCC_SRC)
	$(MKDIRP)
	$(EMCC) $(CFLAGS_WASM) $(CFLAGS_SORTED_FUNCS) $(WRAPPER_DEFINES) -c -o $@ $<

$(BUILD_QUICKJS)/%.o: $(QUICKJS_ROOT)/%.c $(WASM_SYMBOLS) | $(EMCC_SRC)
	$(MKDIRP)
	$(EMCC) $(CFLAGS_WASM) $(EMCC_EXPORTED_FUNCS) $(QUICKJS_DEFINES) -c -o $@ $<

$(BUILD_WRAPPER)/symbols.json:
	$(MKDIRP)
	$(GENERATE_TS) symbols $@

$(BUILD_WRAPPER)/asyncify-remove.json:
	$(MKDIRP)
	$(GENERATE_TS) sync-symbols $@

$(BUILD_WRAPPER)/asyncify-imports.json:
	$(MKDIRP)
	$(GENERATE_TS) async-callback-symbols $@
