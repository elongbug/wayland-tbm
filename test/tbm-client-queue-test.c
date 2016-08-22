#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define WL_HIDE_DEPRECATED
#include <wayland-client.h>
#include <wayland-tbm-client.h>
#include <wayland-tbm-int.h>

#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <tbm_surface_internal.h>

#include "wayland-tbm-test-client-protocol.h"

#define WL_APP_C_LOG(fmt, ...)   fprintf(stderr, "[CLIENT(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__)

typedef struct {
	struct wayland_tbm_client *tbm_client;
	struct wl_tbm_test *wl_tbm_test;
	struct wl_test_surface *surface;
	tbm_surface_queue_h surface_queue;

	int try_draw;
	int exit;
	int count;
} AppInfoClient;

typedef struct {
	tbm_surface_h buffer;
	tbm_surface_queue_h surface_queue;
	struct wl_buffer *wl_buffer;
} AppBuffer;

AppInfoClient gApp;
static void _drawing_surface(AppInfoClient *app);

static void
_create_surface_and_queue(AppInfoClient *app)
{
	app->surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	app->surface_queue = wayland_tbm_client_create_surface_queue(app->tbm_client,
			     (struct wl_surface *)app->surface, 3, 100, 100,
			     TBM_FORMAT_ABGR8888);
	WL_APP_C_LOG("surface:%p, surface_queue:%p\n", app->surface,
		     app->surface_queue);
	return;
}

static void
_destroy_surface_and_queue(AppInfoClient *app)
{
	tbm_surface_queue_destroy(app->surface_queue);
	wl_test_surface_destroy(app->surface);

	WL_APP_C_LOG("surface:%p, surface_queue:%p\n", app->surface,
		     app->surface_queue);

	return;
}

static const int key_app_buffer;

static struct wl_buffer *
_get_wl_buffer(tbm_surface_h buffer)
{
	AppBuffer *app_buffer = NULL;

	tbm_surface_internal_get_user_data(buffer,
					   (unsigned long)&key_app_buffer,
					   (void **)&app_buffer);

	if (!app_buffer)
		return (struct wl_buffer *)NULL;

	return app_buffer->wl_buffer;
}

static void
_set_wl_buffer(tbm_surface_h buffer, AppBuffer *app_buffer)
{
	tbm_surface_internal_add_user_data(buffer,
					   (unsigned long)&key_app_buffer,
					   NULL);
	tbm_surface_internal_set_user_data(buffer,
					   (unsigned long)&key_app_buffer,
					   (void *)app_buffer);
}

static void
_cb_wl_buffer_release_callback(void *data, struct wl_buffer *wl_buffer)
{
	AppBuffer *app_buffer = data;

	tbm_surface_queue_release(app_buffer->surface_queue, app_buffer->buffer);
	WL_APP_C_LOG("RELEASE queue:%p, tbm_surface:%p\n", app_buffer->surface_queue,
		     app_buffer->buffer);
}

static const struct wl_buffer_listener buffer_release_listener = {
	_cb_wl_buffer_release_callback
};

static void
_cb_wl_callback_surface_frame_done(void *data,
				    struct wl_callback *wl_callback,
				    uint32_t callback_data)
{
	AppInfoClient *app = data;

	WL_APP_C_LOG("FRAME DONE\n");
	_drawing_surface(app);
	wl_callback_destroy(wl_callback);
}

static const struct wl_callback_listener surface_callback_listener = {
	_cb_wl_callback_surface_frame_done
};

static void
_drawing_surface(AppInfoClient *app)
{
	tbm_surface_queue_h surface_queue = app->surface_queue;
	tbm_surface_h buffer = NULL;
	tbm_surface_info_s info;
	struct wl_buffer *wl_buffer;
	struct wl_callback *wl_callback;

	app->count++;
	if (app->count == 10)
		app->exit = 1;

	if (!tbm_surface_queue_can_dequeue(surface_queue, 0)) {
		WL_APP_C_LOG("Wait free_buffer\n");
		app->try_draw = 1;
		return;
	} else {
		app->try_draw = 0;
	}

	tbm_surface_queue_dequeue(surface_queue, &buffer);
	if (buffer == NULL) {
		WL_APP_C_LOG("Failed to dequeue_buffer\n");
		return;
	}
	//WL_APP_C_LOG("DEQUEUE queue:%p, tbm_surface:%p\n", app->surface_queue, buffer);

	tbm_surface_map(buffer, TBM_OPTION_READ | TBM_OPTION_WRITE, &info);
	WL_APP_C_LOG("\tDraw: buf:%p\n", buffer);
	tbm_surface_unmap(buffer);
	tbm_surface_queue_enqueue(app->surface_queue, buffer);
	//WL_APP_C_LOG("ENQUEUE queue:%p, tbm_surface:%p\n", app->surface_queue, buffer);

	tbm_surface_queue_acquire(app->surface_queue, &buffer);
	//WL_APP_C_LOG("ACQUIRE queue:%p, tbm_surface:%p\n", app->surface_queue, buffer);
	wl_buffer = _get_wl_buffer(buffer);
	if (!wl_buffer) {
		AppBuffer *app_buffer = calloc(1, sizeof(AppBuffer));

		wl_buffer = wayland_tbm_client_create_buffer(app->tbm_client, buffer);
		wl_buffer_add_listener(wl_buffer, &buffer_release_listener, (void *)app_buffer);

		app_buffer->wl_buffer = wl_buffer;
		app_buffer->surface_queue = app->surface_queue;
		app_buffer->buffer = buffer;
		_set_wl_buffer(buffer, app_buffer);
		WL_APP_C_LOG("\tCreate NEW wl_buffer: buf:%p\n", buffer);
	}

	wl_callback = wl_test_surface_frame(app->surface);
	wl_callback_add_listener(wl_callback, &surface_callback_listener, app);
	wl_test_surface_attach(app->surface, wl_buffer);
	//WL_APP_C_LOG("ATTACH wl_surface:%p, wl_buffer:%p\n", app->surface, wl_buffer);
}

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

int
main(int argc, char *argv[])
{
	struct wl_display *dpy = NULL;
	struct wl_registry *registry;
	const char *dpy_name = NULL;
	const char *default_dpy_name = "queue";
	int ret = 0;
	tbm_bufmgr bufmgr = NULL;

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
	bufmgr = tbm_bufmgr_init(-1);
	tbm_bufmgr_debug_show(bufmgr);

	_create_surface_and_queue(&gApp);
	_drawing_surface(&gApp);
	while (ret >= 0 && gApp.exit == 0) {
		ret = wl_display_dispatch(dpy);
		if (gApp.try_draw)
			_drawing_surface(&gApp);
	}

	tbm_bufmgr_debug_show(bufmgr);
finish:
	if (gApp.surface)
		_destroy_surface_and_queue(&gApp);
	if (bufmgr)
		tbm_bufmgr_debug_show(bufmgr);
	return 1;
}
