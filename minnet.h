#ifndef MINNET_H
#define MINNET_H

#include <quickjs.h>
#include "jsutils.h"

#define SETLOG(max_level) lws_set_log_level(((((max_level) << 1) - 1) & (~LLL_PARSER)) | LLL_USER, NULL);

#define ADD(ptr, inst, member) \
  do { \
    (*(ptr)) = (inst); \
    (ptr) = &(*(ptr))->member; \
  } while(0);

#define LOG(name, fmt, args...) \
  lwsl_user("%-5s" \
            " " fmt "\n", \
            (char*)(name), \
            args);
#define LOGCB(name, fmt, args...) LOG((name), FG("%d") "%-38s" NC " wsi#%" PRId64 " " fmt "", 22 + (reason * 2), lws_callback_name(reason) + 13, opaque ? opaque->serial : -1, args);

typedef enum socket_state MinnetStatus;

void minnet_io_handlers(JSContext*, struct lws* wsi, struct lws_pollargs args, JSValueConst out[2]);
void minnet_log_callback(int, const char* line);
int minnet_lws_unhandled(const char*, int reason);
JSValue minnet_get_sessions(JSContext*, JSValueConst this_val, int argc, JSValueConst argv[]);
JSModuleDef* js_init_module_minnet(JSContext*, const char* module_name);

#endif /* MINNET_H */
