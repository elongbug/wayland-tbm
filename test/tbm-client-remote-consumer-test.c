#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define WL_HIDE_DEPRECATED
#include <wayland-client.h>
#include <wayland-tbm-client.h>
#include <wayland-tbm-int.h>

#include <tbm_surface.h>

#include "wayland-tbm-test-client-protocol.h"

#define WL_APP_C_LOG(fmt, ...)   fprintf(stderr, "[CLIENT(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__)

typedef struct {
	struct wayland_tbm_client *tbm_client;
	struct wl_tbm *wl_tbm;
	struct wl_tbm_test *wl_tbm_test;
	struct wl_test_remote *remote;

	struct wl_buffer *bufs[3];
	int cur_buf;

	int exit;
} AppInfoClient;

AppInfoClient gApp;

static void
_wl_registry_global_cb(void *data,
		       struct wl_registry *wl_registry,
		       uint32_t name,
		       const char *interface,
		       uint32_t version)
{
	AppInfoClient *app = (AppInfoClient *)data;

	if (!strcmp(interface, "wl_tbm_test")) {
		WL_APP_C_LOG("bind %s", interface);
		app->wl_tbm_test = wl_registry_bind(wl_registry, name,
						    &wl_tbm_test_interface, 1);
	}
}

static void
_wl_registry_global_remove_cb(void *data,
			      struct wl_registry *wl_registry,
			      uint32_t name)
{
}

static const struct wl_registry_listener wl_registry_impl = {
	_wl_registry_global_cb,
	_wl_registry_global_remove_cb,
};

static void
_wl_test_remote_update_cb(void *data,
		       struct wl_test_remote *wl_test_remote,
		       struct wl_buffer *buffer)
{
	WL_APP_C_LOG("wl_buffer:%p, tbm_surface:%p\n", buffer, wl_buffer_get_user_data(buffer));
}

static const struct wl_test_remote_listener wl_test_remote_impl = {
	_wl_test_remote_update_cb
};

void create_consumer(AppInfoClient *app)
{
	app->remote = wl_tbm_test_create_remote_surface(app->wl_tbm_test, "test");
	wl_test_remote_add_listener(app->remote, &wl_test_remote_impl, NULL);
	wl_test_remote_redirect(app->remote, app->wl_tbm);
}

int
main(int argc, char *argv[])
{
	static const char *default_dpy_name = "tbm_remote";
	struct wl_display *dpy = NULL;
	struct wl_registry *registry;
	const char *dpy_name = NULL;
	int ret = 0;

	if (argc > 1)
		dpy_name = argv[1];
	else
		dpy_name = default_dpy_name;

	dpy = wl_display_connect(dpy_name);
	if (!dpy) {
		printf("[APP] failed to connect server\n");
		return -1;
	}

	registry = wl_display_get_registry(dpy);
	wl_registry_add_listener(registry, &wl_registry_impl, &gApp);
	wl_display_roundtrip(dpy);
	if (gApp.wl_tbm_test == NULL) {
		WL_APP_C_LOG("fail to bind::wl_tbm_test");
		return 0;
	}

	gApp.tbm_client = wayland_tbm_client_init(dpy);
	if (!gApp.tbm_client) {
		WL_APP_C_LOG("fail to wayland_tbm_client_init()\n");
		goto finish;
	}
	gApp.wl_tbm = wayland_tbm_client_get_wl_tbm(gApp.tbm_client);

	create_consumer(&gApp);

	while (ret >= 0 && gApp.exit == 0)
		ret = wl_display_dispatch(dpy);

finish:
	return 1;
}

