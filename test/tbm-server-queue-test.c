#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WL_HIDE_DEPRECATED
#include <wayland-server.h>
#include <wayland-tbm-server.h>
#include <wayland-tbm-int.h>

#include <tbm_surface.h>
#include <tbm_surface_queue.h>

#include "wayland-tbm-test-server-protocol.h"

#define SERVER_LOG(fmt, ...)   fprintf(stderr, "[SERVER(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__)

#define NUM_SCANOUT_BUFFER 3
#define SCANOUT_BUFFER	0xFF30
#define UPDATE_TIMER	20

typedef struct _AppInfo AppInfo;
typedef struct _AppSurface AppSurface;

struct _AppInfo {
	struct wl_display *dpy;
	struct wl_list list_surface;
	struct wl_global *wl_tbm_test;
	struct wl_event_source *mode_change_timer; //for switch comp_mode
	struct wl_event_source *update_timer; //vertual VSYNC
	struct wl_event_source *idler; //For display update

	struct wayland_tbm_server *tbm_server;

	//For display
	tbm_surface_queue_h scanout_queue;
	struct wayland_tbm_server_queue *server_queue;

	int need_update;
	AppSurface *active_surface;
	uint32_t update_count;

	tbm_surface_h front, back;

	int test_mode;
};

struct _AppSurface {
	struct wl_list link;
	struct wl_resource *resource;
	struct wl_resource *frame_callback;
	AppInfo *app;

	struct wl_resource *front_buffer;
	struct wl_resource *update_buffer;
	int need_frame_done;
};

AppInfo gApp;
static void wl_tbm_test_idle_cb(void *data);

static void
_wl_test_surface_destroy(struct wl_resource *resource)
{
	AppSurface *app_surface = wl_resource_get_user_data(resource);
	if (!app_surface) {
		SERVER_LOG("resource:%p fail get user_data\n", resource);
		return;
	}

	AppInfo *app = app_surface->app;
	SERVER_LOG("resource:%p\n", resource);

	wl_list_remove(&app_surface->link);
	wl_resource_set_user_data(resource, NULL);

	if (app_surface == app->active_surface)
		app->active_surface = NULL;

	free(app_surface);
}


static void
_wl_test_surface_destroy_cb(struct wl_client *client,
			    struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
_wl_test_surface_attach_cb(struct wl_client *client,
			   struct wl_resource *resource,
			   struct wl_resource *wl_buffer)
{
	AppInfo *app;
	AppSurface *app_surface = wl_resource_get_user_data(resource);
	struct wl_resource *old_wl_buffer = app_surface->update_buffer;

	app = app_surface->app;
	app_surface->update_buffer = wl_buffer;

	if (!app->idler) {
		app->idler = wl_event_loop_add_idle(wl_display_get_event_loop(app->dpy),
						    wl_tbm_test_idle_cb, app);
	}

	if (old_wl_buffer)
		wl_buffer_send_release(old_wl_buffer);
}

static void
_wl_test_surface_frame_cb(struct wl_client *client,
			  struct wl_resource *resource,
			  uint32_t callback)
{
	AppSurface *app_surface = wl_resource_get_user_data(resource);
	struct wl_resource *done;

	done = wl_resource_create(client, &wl_callback_interface, 1, callback);
	if (app_surface->frame_callback)
		wl_resource_destroy(app_surface->frame_callback);

	app_surface->frame_callback = done;
}

static const struct wl_test_surface_interface wl_test_surface_impl = {
	_wl_test_surface_destroy_cb,
	_wl_test_surface_attach_cb,
	_wl_test_surface_frame_cb
};

static void
_wl_tbm_test_create_surface(struct wl_client *client,
			    struct wl_resource *resource,
			    uint32_t surface)
{
	AppInfo *app = (AppInfo *)wl_resource_get_user_data(resource);
	AppSurface *app_surface;
	struct wl_resource *test_surface;

	app_surface = calloc(1, sizeof(AppSurface));

	test_surface = wl_resource_create(client, &wl_test_surface_interface, 1,
					  surface);
	wl_resource_set_implementation(test_surface,
				       &wl_test_surface_impl,
				       app_surface,
				       _wl_test_surface_destroy);

	wl_list_insert(&app->list_surface, &app_surface->link);

	app_surface->app = app;
	app_surface->resource = test_surface;
	SERVER_LOG("Add surface:%p\n", test_surface);
}

static void
_wl_tbm_test_set_active_surface(struct wl_client *client,
			    struct wl_resource *resource,
			    struct wl_resource *surface)
{
	AppInfo *app = (AppInfo *)wl_resource_get_user_data(resource);
	AppSurface *app_surface = (AppSurface *)wl_resource_get_user_data(surface);

	wayland_tbm_server_queue_set_surface(app->server_queue,
			surface, 0x1111);
	app->active_surface = app_surface;

	SERVER_LOG("Active surface:%p\n", app_surface);
}

static const struct wl_tbm_test_interface wl_tbm_test_impl = {
	_wl_tbm_test_create_surface,
	_wl_tbm_test_set_active_surface
};

static void
wl_tbm_test_bind_cb(struct wl_client *client, void *data,
		    uint32_t version, uint32_t id)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, &wl_tbm_test_interface, version, id);
	wl_resource_set_implementation(resource, &wl_tbm_test_impl, data, NULL);

	SERVER_LOG("client:%p, wl_tbm_test:%p\n", client, resource);
}

static int
wl_tbm_test_mode_timer_cb(void *data)
{
//	AppInfo* app = (AppInfo*)data;

	return 1;
}

static int
wl_tbm_test_update_timer_cb(void *data)
{
	AppInfo *app = (AppInfo *)data;
	AppSurface *app_surface, *tmp;

	if (!app->need_update) {
		wl_event_source_timer_update(app->update_timer, UPDATE_TIMER);
		return 1;
	}
	app->need_update = 0;

	SERVER_LOG("VSYNC, PageFlipComplete~\n");
	if (app->front)
		tbm_surface_queue_release(app->scanout_queue, app->front);

	app->front = app->back;
	app->back = NULL;

	wl_list_for_each_reverse_safe(app_surface, tmp, &app->list_surface, link) {
		if (app_surface->need_frame_done) {
			app_surface->need_frame_done = 0;

			if (app_surface->frame_callback) {
				wl_callback_send_done(app_surface->frame_callback, 0);
				wl_resource_destroy(app_surface->frame_callback);
				app_surface->frame_callback = NULL;
			}
		}
	}

	wl_event_source_timer_update(app->update_timer, UPDATE_TIMER);
	return 1;
}

static void
wl_tbm_test_idle_cb(void *data)
{
	AppInfo *app = data;
	AppSurface *app_surface, *tmp;
	tbm_surface_h back = NULL;

	SERVER_LOG("\n");

	app->idler = NULL;

	if (app->active_surface) {
		if (app->active_surface->update_buffer) {
			tbm_surface_h buffer;
			uint32_t flags;
			buffer = wayland_tbm_server_get_surface(NULL,
								app->active_surface->update_buffer);
			flags = wayland_tbm_server_get_flags(NULL, app->active_surface->update_buffer);
			if (flags == SCANOUT_BUFFER) {
				tbm_surface_queue_enqueue(app->scanout_queue, buffer);
				app->active_surface->update_buffer = NULL;
				app->active_surface->need_frame_done = 1;
				SERVER_LOG("\tDirect present\n");
				goto present;
			}
		}
	}

	//DO Composite
	wl_list_for_each_reverse_safe(app_surface, tmp, &app->list_surface, link) {
		tbm_surface_h buffer;
		uint32_t flags;

		if (app_surface->update_buffer == NULL)
			continue;

		buffer = wayland_tbm_server_get_surface(NULL, app_surface->update_buffer);
		flags = wayland_tbm_server_get_flags(NULL, app_surface->update_buffer);

		SERVER_LOG("Composite %p buffer:%p\n", app_surface, app_surface->update_buffer);
		if (flags == SCANOUT_BUFFER)
			tbm_surface_queue_release(app->scanout_queue, buffer);
		else
			wl_buffer_send_release(app_surface->update_buffer);

		app_surface->update_buffer = NULL;
		app_surface->need_frame_done = 1;
	}

	tbm_surface_queue_dequeue(app->scanout_queue, &back);
	if (back == NULL) {
		SERVER_LOG("NO_FREE_BUFFER for comp\n");
		return;
	}
	tbm_surface_queue_enqueue(app->scanout_queue, back);

present:
	if (app->test_mode == 1) {
		app->update_count++;
		if (!(app->update_count % 5)) {
			SERVER_LOG("MODE_CHANGE active:%p\n", app->active_surface);
			if (app->active_surface) {
				wayland_tbm_server_queue_set_surface(app->server_queue,
								     NULL,
								     0);
				app->active_surface = NULL;
			} else {
				if (!wl_list_empty(&app->list_surface)) {
					app_surface = wl_container_of(app->list_surface.next, app_surface, link);
					if (wayland_tbm_server_queue_set_surface(app->server_queue,
							app_surface->resource, 1)) {
						SERVER_LOG("!! ERROR wayland_tbm_server_queue_set_surface\n");
					} else {
						app->active_surface = app_surface;
					}
				}
			}
		}
	}

	//Present to LCD
	if (app->back) {
		SERVER_LOG("SKIP this frame\n");
		tbm_surface_queue_release(app->scanout_queue, app->back);
		app->back = NULL;
	}

	tbm_surface_queue_acquire(app->scanout_queue, &app->back);
	if (app->back) {
		SERVER_LOG("need_update\n");
		app->need_update = 1;
	}
}

int
main(int argc, char *argv[])
{
	struct wl_display *dpy;
	tbm_surface_h init_front;

	const char *dpy_name = "queue";

	if (argc > 1) {
		SERVER_LOG("TEST_MODE: %s\n", argv[1]);
		gApp.test_mode = atoi(argv[1]);
	}

	dpy = wl_display_create();
	if (!dpy) {
		printf("[SRV] failed to create display\n");
		return -1;
	}

	wl_display_add_socket(dpy, dpy_name);
	printf("[SRV] wl_display : %s\n", dpy_name);

	wl_list_init(&gApp.list_surface);

	setenv("WL_TBM_EMBEDDED_SERVER", "true", 1);
	gApp.dpy = dpy;
	gApp.tbm_server = wayland_tbm_server_init(dpy, NULL, -1, 0);
	if (!gApp.tbm_server) {
		printf("[SRV] failed to tbm_server init\n");
		wl_display_destroy(dpy);
		return -1;
	}

	gApp.wl_tbm_test = wl_global_create(dpy, &wl_tbm_test_interface, 1, &gApp,
					    wl_tbm_test_bind_cb);

	gApp.scanout_queue = tbm_surface_queue_create(NUM_SCANOUT_BUFFER, 100, 100,
			     TBM_FORMAT_ABGR8888, 0);
	gApp.server_queue = wayland_tbm_server_create_queue(gApp.tbm_server,
			    gApp.scanout_queue, SCANOUT_BUFFER);

	tbm_surface_queue_dequeue(gApp.scanout_queue, &init_front);
	/*TODO : Clear Screen*/
	tbm_surface_queue_enqueue(gApp.scanout_queue, init_front);
	tbm_surface_queue_acquire(gApp.scanout_queue, &init_front);
	gApp.front = init_front;

	gApp.mode_change_timer = wl_event_loop_add_timer(wl_display_get_event_loop(dpy),
				 wl_tbm_test_mode_timer_cb, &gApp);
	gApp.update_timer = wl_event_loop_add_timer(wl_display_get_event_loop(dpy),
			    wl_tbm_test_update_timer_cb, &gApp);
	wl_event_source_timer_update(gApp.update_timer, UPDATE_TIMER);

	wl_display_run(dpy);

	return 0;
}
