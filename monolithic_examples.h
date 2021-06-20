
#pragma once

#if defined(BUILD_MONOLITHIC)

#ifdef __cplusplus
extern "C" {
#endif

int qjscompress_main(int argc, const char* argv[]);
int qjs_main(int argc, const char* argv[]);
int qjsc_main(int argc, const char* argv[]);
int qjs_unicode_gen_main(int argc, const char* argv[]);
int qjs_test262_main(int argc, const char* argv[]);

#ifdef __cplusplus
}
#endif

#endif
