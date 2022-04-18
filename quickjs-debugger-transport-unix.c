#if !defined(WIN32) && !defined(WIN64)

#include "quickjs-debugger.h"
#include "quickjs-debugger-transport.h"
#include "cutils.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>

struct js_transport_data {
    int handle;
} js_transport_data;

static int js_transport_read(void *udata, char *buffer, int length) {
    struct js_transport_data* data = (struct js_transport_data *)udata;
    if (data->handle <= 0)
        return -1;

    if (length <= 0)
        return -2;

    if (buffer == NULL)
        return -3;

    ssize_t ret = read(data->handle, (void *)buffer, length);
    if (ret < 0)
        return -4;

    if (ret == 0)
        return -5;

    if (ret > length)
        return -6;

    return ret;
}

static int js_transport_write(void *udata, const char *buffer, int length) {
    struct js_transport_data* data = (struct js_transport_data *)udata;
    if (data->handle <= 0)
        return -1;

    if (length <= 0)
        return -2;

    if (buffer == NULL)
        return -3;

    int ret = write(data->handle, (const void *) buffer, length);
    if (ret <= 0 || ret > length)
        return -4;

    return ret;
}

static int js_transport_peek(void *udata) {
    struct pollfd fds[1];
    int poll_rc;

    struct js_transport_data* data = (struct js_transport_data *)udata;
    if (data->handle <= 0)
        return -1;

    fds[0].fd = data->handle;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    poll_rc = poll(fds, 1, 0);
    if (poll_rc < 0)
        return -2;
    if (poll_rc > 1)
        return -3;
    // no data
    if (poll_rc == 0)
        return 0;
    // has data
    return 1;
}

static void js_transport_close(JSRuntime* rt, void *udata) {
    struct js_transport_data* data = (struct js_transport_data *)udata;
    if (data->handle <= 0)
        return;
    close(data->handle);
    data->handle = 0;
    free(udata);
}

// todo: fixup asserts to return errors.
static int js_debugger_parse_sockaddr(const char* address, struct sockaddr_in *addr_ref) {
    const char* port_string = strstr(address, ":");
    if (!port_string)
		return -1;

    int port = atoi(port_string + 1);
    if (port <= 0)
		return -2;

    char host_string[256];
    strcpy(host_string, address);
    host_string[port_string - address] = 0;

    struct hostent *host = gethostbyname(host_string);
    if (!host)
		return -3;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy((char *)&addr.sin_addr.s_addr, (char *)host->h_addr, host->h_length);
    addr.sin_port = htons(port);

	if (addr_ref) {
		*addr_ref = addr;
		return 0;
	}
	return -4;
}

void js_debugger_connect(JSContext *ctx, const char *address) {
    struct sockaddr_in addr;
    int parse_addr_result = js_debugger_parse_sockaddr(address, &addr);
    QJS_ASSERT(parse_addr_result == 0);

    int client = socket(AF_INET, SOCK_STREAM, 0);
    QJS_ASSERT(client >= 0);

	int h = connect(client, (const struct sockaddr*)&addr, sizeof(addr));
    QJS_ASSERT(!h);

    struct js_transport_data *data = (struct js_transport_data *)malloc(sizeof(struct js_transport_data));
    memset(data, 0, sizeof(js_transport_data));
    data->handle = client;
    js_debugger_attach(ctx, js_transport_read, js_transport_write, js_transport_peek, js_transport_close, data);
}

void js_debugger_wait_connection(JSContext *ctx, const char* address) {
    struct sockaddr_in addr;
    int parse_addr_result = js_debugger_parse_sockaddr(address, &addr);
    QJS_ASSERT(parse_addr_result == 0);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    QJS_ASSERT(server >= 0);

    int reuseAddress = 1;
    int rv = setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuseAddress, sizeof(reuseAddress));
	QJS_ASSERT(rv >= 0);

    rv = bind(server, (struct sockaddr *) &addr, sizeof(addr));
	QJS_ASSERT(rv >= 0);

    listen(server, 1);

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = (socklen_t) sizeof(addr);
    int client = accept(server, (struct sockaddr *) &client_addr, &client_addr_size);
    close(server);
    QJS_ASSERT(client >= 0);

    struct js_transport_data *data = (struct js_transport_data *)malloc(sizeof(struct js_transport_data));
    memset(data, 0, sizeof(js_transport_data));
    data->handle = client;
    js_debugger_attach(ctx, js_transport_read, js_transport_write, js_transport_peek, js_transport_close, data);
}

#endif
