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
	struct wl_test_surface *surface;

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

void create_provider(AppInfoClient *app)
{
	tbm_surface_h tbm_surf;
	int i;

	app->surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	wl_tbm_test_set_provider(app->wl_tbm_test, app->surface, "test");

	//Make wl_buffer
	for (i = 0; i < 3; i++) {
		tbm_surf = tbm_surface_create(16, 16, TBM_FORMAT_ABGR8888);
		app->bufs[i] = wayland_tbm_client_create_buffer(app->tbm_client, tbm_surf);
	}
}

void _wl_callback_done_cb(void *data,
		     struct wl_callback *wl_callback,
		     uint32_t callback_data)
{
	AppInfoClient *app = (AppInfoClient *)data;

	wl_test_surface_attach(app->surface, app->bufs[app->cur_buf]);
	app->cur_buf = (app->cur_buf + 1) % 3;

	wl_callback_destroy(wl_callback);
}

static struct wl_callback_listener do_done_impl = {
	_wl_callback_done_cb
};

int
main(int argc, char *argv[])
{
	struct wl_display *dpy = NULL;
	struct wl_registry *registry;
	const char *dpy_name = NULL;
	static const char *default_dpy_name = "tbm_remote";
	int ret = 0;

	struct wl_callback *wl_cb;

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

	create_provider(&gApp);

	while (ret >= 0 && gApp.exit == 0) {
		wl_cb = wl_display_sync(dpy);
		wl_callback_add_listener(wl_cb, &do_done_impl, &gApp);
		ret = wl_display_dispatch(dpy);
	}

finish:
	return 1;
}
