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

#define WL_APP_LOG(fmt, ...)   fprintf(stderr, "[CLIENT(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__)
#define WL_APP_CHECK(cond) {\
	if (!(cond)) {\
		WL_APP_LOG("[%s:%d] : '%s' failed.\n", __FUNCTION__, __LINE__, #cond);\
	} \
}

typedef struct {
	struct wl_display *dpy;
	struct wayland_tbm_client *tbm_client;
	struct wl_tbm_test *wl_tbm_test;
	tbm_bufmgr bufmgr;
} AppInfoClient;

static AppInfoClient gApp;

static void
_wl_registry_global_cb(void *data,
		       struct wl_registry *wl_registry,
		       uint32_t name,
		       const char *interface,
		       uint32_t version)
{
	AppInfoClient *app = (AppInfoClient *)data;

	if (!strcmp(interface, "wl_tbm_test")) {
		WL_APP_LOG("bind %s", interface);
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

static int test_01(AppInfoClient* app);
static int test_02(AppInfoClient* app);
static int test_03(AppInfoClient* app);
static int test_04(AppInfoClient* app);
static int test_05(AppInfoClient* app);
static int test_06(AppInfoClient* app);

int
main(int argc, char *argv[])
{
	const char *default_dpy_name = "queue";
	struct wl_registry *registry;
	const char *dpy_name = NULL;

	if (argc > 1)
		dpy_name = argv[1];
	else
		dpy_name = default_dpy_name;

	gApp.dpy = wl_display_connect(dpy_name);
	if (!gApp.dpy) {
		printf("[APP] failed to connect server\n");
		return -1;
	}

	registry = wl_display_get_registry(gApp.dpy);
	wl_registry_add_listener(registry, &wl_registry_impl, &gApp);
	wl_display_roundtrip(gApp.dpy);
	if (gApp.wl_tbm_test == NULL) {
		WL_APP_LOG("fail to bind::wl_tbm_test");
		return 0;
	}

	gApp.tbm_client = wayland_tbm_client_init(gApp.dpy);
	if (!gApp.tbm_client) {
		WL_APP_LOG("fail to wayland_tbm_client_init()\n");
		goto finish;
	}
	gApp.bufmgr = tbm_bufmgr_init(-1);
	tbm_bufmgr_debug_show(gApp.bufmgr);

	test_01(&gApp);
	test_02(&gApp);
	test_03(&gApp);
	test_04(&gApp);
	test_05(&gApp);
	test_06(&gApp);


finish:
	if (gApp.bufmgr) {
		tbm_bufmgr_debug_show(gApp.bufmgr);
		tbm_bufmgr_deinit(gApp.bufmgr);
	}

	return 1;
}
static int
print_mem_info(AppInfoClient* app, char *str)
{
	WL_APP_LOG("## %s ##\n", str);
	tbm_bufmgr_debug_show(app->bufmgr);
	WL_APP_LOG("\n");

	return 0;
}

static int
test_01(AppInfoClient* app)
{
	struct wl_test_surface* surface;
	tbm_surface_queue_h surface_queue;
	tbm_surface_h buffer = NULL;

	surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	surface_queue = wayland_tbm_client_create_surface_queue(app->tbm_client,
					(struct wl_surface*)surface,
					3, 100, 100, TBM_FORMAT_ABGR8888);

	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);
	tbm_surface_queue_enqueue(surface_queue, buffer);
	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);
	tbm_surface_queue_enqueue(surface_queue, buffer);

	tbm_surface_queue_destroy(surface_queue);
	wl_test_surface_destroy(surface);
	wl_display_roundtrip(app->dpy);

	print_mem_info(app, "TEST_01");

	return 0;
}

static int
test_02(AppInfoClient* app)
{
	struct wl_test_surface* surface;
	tbm_surface_queue_h surface_queue;
	tbm_surface_h buffer = NULL;

	surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	surface_queue = wayland_tbm_client_create_surface_queue(app->tbm_client,
					(struct wl_surface*)surface,
					3, 100, 100, TBM_FORMAT_ABGR8888);

	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);
	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);

	tbm_surface_queue_destroy(surface_queue);
	wl_test_surface_destroy(surface);
	wl_display_roundtrip(app->dpy);

	print_mem_info(app, "TEST_02");

	return 0;
}

static int
test_03(AppInfoClient* app)
{
	struct wl_test_surface* surface;
	tbm_surface_queue_h surface_queue;
	tbm_surface_h buffer = NULL;

	surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	surface_queue = wayland_tbm_client_create_surface_queue(app->tbm_client,
					(struct wl_surface*)surface,
					3, 100, 100, TBM_FORMAT_ABGR8888);

	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);
	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);

	tbm_surface_queue_reset(surface_queue, 200, 200, TBM_FORMAT_ABGR8888);

	tbm_surface_queue_destroy(surface_queue);
	wl_test_surface_destroy(surface);
	wl_display_roundtrip(app->dpy);

	print_mem_info(app, "TEST_03");

	return 0;
}

static int
test_04(AppInfoClient* app)
{
	struct wl_test_surface* surface;
	tbm_surface_queue_h surface_queue;
	tbm_surface_h buffer = NULL;

	surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	surface_queue = wayland_tbm_client_create_surface_queue(app->tbm_client,
					(struct wl_surface*)surface,
					3, 100, 100, TBM_FORMAT_ABGR8888);

	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);
	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);

	tbm_surface_queue_reset(surface_queue, 200, 200, TBM_FORMAT_ABGR8888);
	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);
	tbm_surface_queue_enqueue(surface_queue, buffer);
	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);
	tbm_surface_internal_ref(buffer);

	tbm_surface_queue_destroy(surface_queue);
	wl_test_surface_destroy(surface);
	wl_display_roundtrip(app->dpy);

	print_mem_info(app, "TEST_04");
	tbm_surface_internal_unref(buffer);
	print_mem_info(app, "TEST_04_final");

	return 0;
}

static int
test_05(AppInfoClient* app)
{
	struct wl_test_surface* surface;
	tbm_surface_queue_h surface_queue;
	tbm_surface_h buffer = NULL;
	struct wl_buffer* wl_buffer;

	surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	surface_queue = wayland_tbm_client_create_surface_queue(app->tbm_client,
					(struct wl_surface*)surface,
					3, 100, 100, TBM_FORMAT_ABGR8888);

	tbm_surface_queue_dequeue(surface_queue, &buffer);
	WL_APP_CHECK(buffer != NULL);

	wl_buffer = wayland_tbm_client_create_buffer(app->tbm_client, buffer);
	wl_test_surface_attach(surface, wl_buffer);
	wl_display_roundtrip(app->dpy);

	tbm_surface_queue_destroy(surface_queue);
	wl_test_surface_destroy(surface);
	wl_display_roundtrip(app->dpy);

	print_mem_info(app, "TEST_05");

	return 0;
}

static int
test_06(AppInfoClient* app)
{
	struct wl_test_surface* surface;
	tbm_surface_queue_h surface_queue;

	WL_APP_LOG("\n");
	surface = wl_tbm_test_create_surface(app->wl_tbm_test);
	surface_queue = wayland_tbm_client_create_surface_queue(app->tbm_client,
					(struct wl_surface*)surface,
					3, 100, 100, TBM_FORMAT_ABGR8888);

	wl_tbm_test_set_active_queue(app->wl_tbm_test, surface);
	wl_display_roundtrip(app->dpy);

	tbm_surface_queue_destroy(surface_queue);
	wl_test_surface_destroy(surface);
	wl_display_roundtrip(app->dpy);

	print_mem_info(app, "TEST_06");

	return 0;
}

